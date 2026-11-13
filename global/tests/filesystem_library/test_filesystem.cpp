// global
#include "global/tests/forward.hpp"

// global
#include "global/filesystem_library/filesystem.hpp"


namespace global::filesystem
{

namespace
{

namespace fs = std::filesystem;

path_t make_unique_glob_root()
{
	const auto root = path_to_global_test_data / "filesystem_library/glob_recurse_files";
	fs::remove_all(root);
	fs::create_directories(root / "sub_1");
	fs::create_directories(root / "sub_2" / "sub_3");
	return root;
}

void create_fake_tree(const path_t& root)
{
	std::ofstream(root / "sub_1" / "a.txt", std::ios::out | std::ios::binary) << "a";
	std::ofstream(root / "sub_1" / "b.png", std::ios::out | std::ios::binary) << "b";
	std::ofstream(root / "sub_2" / "c.txt", std::ios::out | std::ios::binary) << "c";
	std::ofstream(root / "sub_2" / "sub_3" / "d.xml", std::ios::out | std::ios::binary) << "d";
}

abstract::filesystem_t make_abstract_filesystem()
{
	const auto physical_filesystem = physically::make_filesystem();
	return {
		.exist = physical_filesystem.exist,
		.is_regular_file = physical_filesystem.is_regular_file,
		.get_recursive_directory_range = physical_filesystem.get_recursive_directory_range
	};
}

}// namespace

TEST_SUITE("global/filesystem_library/filesystem")
{

const auto path_to_suite_test_data = path_to_global_test_data / "filesystem_library/";
const auto filesystem = physically::make_filesystem();

TEST_CASE("try_read")
{
	const auto path_to_test_data = path_to_suite_test_data / "try_read_file/";
	path_t logged_path;

	SUBCASE("found")
	{
		const auto actual = filesystem.try_read(
		{
			.path = path_to_test_data / "file_1.txt",
			.logger = [&](const auto& path)
			{
				logged_path = path;
			}
		});

		CHECK(actual.has_value());
		CHECK(**actual == "try_read_file 1");
		CHECK(logged_path == path_to_test_data / "file_1.txt");
	}

	SUBCASE("not found")
	{
		const auto actual = filesystem.try_read(
		{
			.path = path_to_test_data / "file_2.txt",
			.logger = [&](const auto& path)
			{
				logged_path = path;
			}
		});

		CHECK_FALSE(actual.has_value());
		CHECK(logged_path == path_to_test_data / "file_2.txt");
	}
}

TEST_CASE("read")
{
	const auto path_to_test_data = path_to_suite_test_data / "read_file/";
	path_t logged_path;
	path_t throw_path;

	SUBCASE("found")
	{
		const auto actual = filesystem.read(
		{
			.path = path_to_test_data / "file_1.txt",
			.logger = [&](const auto& path)
			{
				logged_path = path;
			},
			.thrower = [&](const auto& path)
			{
				throw_path = path;
			}
		});

		CHECK(*actual == "read_file 1");
		CHECK(logged_path == path_to_test_data / "file_1.txt");
		CHECK(throw_path.empty());
	}

	SUBCASE("not found")
	{
		CHECK_THROWS_WITH(
			filesystem.read(
			{
				.path = path_to_test_data / "file_2.txt",
				.logger = [&](const auto& path)
				{
					logged_path = path;
				},
				.thrower = [&](const auto& path)
				{
					throw_path = path;
					throw std::runtime_error("read exception");
				}
			}),
			"read exception"
		);
		CHECK(logged_path == path_to_test_data / "file_2.txt");
		CHECK(throw_path == path_to_test_data / "file_2.txt");
	}
}

TEST_CASE("write")
{
	const auto path_to_test_data = path_to_suite_test_data / "write_file/";
	path_t logged_path;

	filesystem.write(
	{
		.path = path_to_test_data / actual_folder / "file_1.txt",
		.content = make_file_content("write_file"),
		.logger = [&](const auto& path)
		{
			logged_path = path;
		}
	});

	CHECK(logged_path == path_to_test_data / actual_folder / "file_1.txt");
	CHECK(filesystem.compare_directories({
		.path_1 = path_to_test_data / actual_folder,
		.path_2 = path_to_test_data / expected_folder
	}));
}

TEST_CASE("remove")
{
	const auto path_to_test_data = path_to_suite_test_data / "remove/";
	path_t logged_path;

	filesystem.remove(
	{
		.path = path_to_test_data / actual_folder / "for_remove.txt",
		.logger = [&](const auto& path)
		{
			logged_path = path;
		}
	});

	CHECK(logged_path == path_to_test_data / actual_folder / "for_remove.txt");
	CHECK_FALSE(filesystem.exist(path_to_test_data / actual_folder / "for_remove.txt"));
}

TEST_CASE("create_directory")
{
	const auto path_to_test_data = path_to_suite_test_data / "create_directory/";
	path_t logged_path;

	filesystem.create_directory(
	{
		.path = path_to_test_data / actual_folder / "aaa/",
		.logger = [&](const auto& path)
		{
			logged_path = path;
		}
	});

	CHECK(logged_path == path_to_test_data / actual_folder / "aaa/");
	CHECK(filesystem.exist(path_to_test_data / actual_folder / "aaa/"));
}

TEST_CASE("exist")
{
	const auto path_to_test_data = path_to_suite_test_data / "exist/";

	CHECK(filesystem.exist(path_to_test_data / "file_1.txt"));
	CHECK_FALSE(filesystem.exist(path_to_test_data / "file_2.txt"));
}

TEST_CASE("is_regular_file")
{
	const auto path_to_test_data = path_to_suite_test_data / "exist/";

	CHECK(filesystem.is_regular_file(path_to_test_data / "file_1.txt"));
	CHECK_FALSE(filesystem.is_regular_file(path_to_test_data));
	CHECK_FALSE(filesystem.is_regular_file(path_to_test_data / "file_2.txt"));
}

TEST_CASE("to_absolute")
{
	const auto path_to_test_data = path_to_suite_test_data / "to_absolute/";

	CHECK(filesystem.to_absolute(path_to_test_data) == path_to_test_data);
}

TEST_CASE("has_same_root_subpath")
{
	const auto path_to_test_data = path_to_suite_test_data / "has_same_root_subpath/";

	CHECK(filesystem.has_same_root_subpath(
	{
		.path = path_to_test_data,
		.path_root = path_to_suite_test_data
	}));
	CHECK_FALSE(filesystem.has_same_root_subpath(
	{
		.path = path_to_suite_test_data,
		.path_root = path_to_test_data
	}));
}

TEST_CASE("relative paths")
{
	const auto path_to_test_data = path_to_suite_test_data / "get_relative_path_with_filename/";

	CHECK(filesystem.get_relative_path_without_filename(
	{
		.path = path_to_suite_test_data / "get_relative_path_without_filename/file_1.txt",
		.path_root = path_to_suite_test_data
	}) == "get_relative_path_without_filename");
	CHECK(filesystem.get_relative_path_with_filename(
	{
		.path = path_to_test_data / "file_1.txt",
		.path_root = path_to_suite_test_data
	}) == "get_relative_path_with_filename/file_1.txt");
}

TEST_CASE("split_path_by_parts")
{
	const auto path_to_test_data = path_to_suite_test_data / "split_path_by_parts/";
	const auto actual = filesystem.split_path_by_parts(
	{
		.path_root = path_to_suite_test_data,
		.path = path_to_test_data / "nested/file_2.xml"
	});

	CHECK(actual.path == path_to_test_data / "nested/file_2.xml");
	CHECK(actual.root == path_to_suite_test_data);
	CHECK(actual.relative_and_filename == "split_path_by_parts/nested/file_2.xml");
	CHECK(actual.relative == "split_path_by_parts/nested/");
	CHECK(actual.filename.name == "file_2");
	CHECK(actual.filename.extension == ".xml");
}

TEST_CASE("merge_paths")
{
	const auto path_to_suite = path_to_suite_test_data / "merge_paths/";
	const auto old_parts = filesystem.split_path_by_parts(
	{
		.path_root = path_to_suite_test_data,
		.path = path_to_suite / "old_dir/old_file.txt"
	});

	SUBCASE("relative and filename")
	{
		const auto actual = filesystem.merge_paths(
		{
			.old_parts = old_parts,
			.new_parts = {
				.root = path_to_suite_test_data,
				.relative_and_filename = path_t{"new_dir/new_file.txt"}
			},
			.point_1 = [](const auto&) {},
			.point_2 = [](const auto&) {}
		});

		CHECK(actual == path_to_suite_test_data / "new_dir/new_file.txt");
	}

	SUBCASE("relative and filename parts")
	{
		const auto actual = filesystem.merge_paths(
		{
			.old_parts = old_parts,
			.new_parts = {
				.root = path_to_suite_test_data,
				.relative = path_t{"new_dir"},
				.filename = {
					.name = "new_file",
					.extension = ".xml"
				}
			},
			.point_1 = [](const auto&) {},
			.point_2 = [](const auto&) {}
		});

		CHECK(actual == path_to_suite_test_data / "new_dir/new_file.xml");
	}

	SUBCASE("missing root reports point 1")
	{
		CHECK_THROWS_WITH(
			filesystem.merge_paths(
			{
				.old_parts = {},
				.new_parts = {},
				.point_1 = [](const auto& what)
				{
					throw std::runtime_error(what);
				},
				.point_2 = [](const auto&) {}
			}),
			"root must be present in old or new path"
		);
	}

	SUBCASE("missing name reports point 2")
	{
		CHECK_THROWS_WITH(
			filesystem.merge_paths(
			{
				.old_parts = {
					.root = path_to_suite_test_data,
					.relative = path_t{"old_dir"}
				},
				.new_parts = {},
				.point_1 = [](const auto&) {},
				.point_2 = [](const auto& what)
				{
					throw std::runtime_error(what);
				}
			}),
			"name must be present in old or new path"
		);
	}
}

TEST_CASE("compare_directories dir1 dir2")
{
	const auto path_to_test_data = path_to_suite_test_data / "compare_directories_var_1/";

	SUBCASE("equal")
	{
		const auto actual = filesystem.compare_directories({
			.path_1 = path_to_test_data / "test_1" / "dir_1",
			.path_2 = path_to_test_data / "test_1" / "dir_2"
		});

		CHECK(actual);
		CHECK(actual.diff.empty());
	}

	SUBCASE("not equal")
	{
		const auto actual = filesystem.compare_directories({
			.path_1 = path_to_test_data / "test_2" / "dir_1",
			.path_2 = path_to_test_data / "test_2" / "dir_2"
		});

		CHECK_FALSE(actual);
		CHECK_FALSE(actual.diff.empty());
	}
}

TEST_CASE("compare_directories dir")
{
	const auto path_to_test_data = path_to_suite_test_data / "compare_directories_var_2/";

	SUBCASE("equal")
	{
		const auto actual = filesystem.compare_directories({
			.path_1 = path_to_test_data / "test_1" / actual_folder,
			.path_2 = path_to_test_data / "test_1" / expected_folder
		});

		CHECK(actual);
		CHECK(actual.diff.empty());
	}

	SUBCASE("not equal")
	{
		const auto actual = filesystem.compare_directories({
			.path_1 = path_to_test_data / "test_2" / actual_folder,
			.path_2 = path_to_test_data / "test_2" / expected_folder
		});

		CHECK_FALSE(actual);
		CHECK_FALSE(actual.diff.empty());
	}

	SUBCASE("bad actual dir name")
	{
		const auto actual = filesystem.compare_directories({
			.path_1 = path_to_test_data / "test_3" / actual_folder,
			.path_2 = path_to_test_data / "test_3" / expected_folder
		});

		CHECK_FALSE(actual);
		CHECK_FALSE(actual.diff.empty());
	}
}

TEST_CASE("returns recursive matching files")
{
	const auto root = make_unique_glob_root();
	create_fake_tree(root);
	const auto abstract_filesystem = make_abstract_filesystem();

	const auto detail_args = physically::views::glob_recurse_files_t{
		.filesystem = abstract_filesystem,
		.extentions = { ".txt", ".png" }
	};

	const auto actual = physically::views::glob_recurse_files(detail_args)(ranges::views::single(root))
		| ranges::to<std::set<path_t>>
		;

	CHECK(actual.size() == 3);
	CHECK(actual.contains(root / "sub_1" / "a.txt"));
	CHECK(actual.contains(root / "sub_1" / "b.png"));
	CHECK(actual.contains(root / "sub_2" / "c.txt"));
	CHECK_FALSE(actual.contains(root / "sub_2" / "sub_3" / "d.xml"));

	fs::remove_all(root);
}

TEST_CASE("filters by extension")
{
	const auto root = make_unique_glob_root();
	create_fake_tree(root);
	const auto abstract_filesystem = make_abstract_filesystem();

	const auto detail_args = physically::views::glob_recurse_files_t{
		.filesystem = abstract_filesystem,
		.extentions = { ".txt" }
	};

	const auto actual = physically::views::glob_recurse_files(detail_args)(ranges::views::single(root))
		| ranges::to<std::set<path_t>>
		;

	CHECK(actual.size() == 2);
	CHECK(actual.contains(root / "sub_1" / "a.txt"));
	CHECK(actual.contains(root / "sub_2" / "c.txt"));
	CHECK_FALSE(actual.contains(root / "sub_1" / "b.png"));
	CHECK_FALSE(actual.contains(root / "sub_2" / "sub_3" / "d.xml"));

	fs::remove_all(root);
}

TEST_CASE("invalid root throws by is_path_exist")
{
	const auto root = path_to_global_test_data / "executable_library/does_not_exist";

	auto abstract_filesystem = make_abstract_filesystem();
	abstract_filesystem.exist = [](const path_t& path)
	{
		if (!fs::exists(path))
		{
			throw std::runtime_error("missing root");
		}
		return true;
	};
	const auto detail_args = physically::views::glob_recurse_files_t{
		.filesystem = abstract_filesystem,
		.extentions = {}
	};

	CHECK_THROWS(
		physically::views::glob_recurse_files(detail_args)(ranges::views::single(root))
		| ranges::to<std::vector<path_t>>
	);
}

}

}// namespace global::filesystem
